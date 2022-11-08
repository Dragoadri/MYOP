#include "tools/cabana/binaryview.h"

#include <QApplication>
#include <QFontDatabase>
#include <QHeaderView>
#include <QMouseEvent>
#include <QPainter>
#include <QToolTip>

#include "tools/cabana/canmessages.h"

// BinaryView

const int CELL_HEIGHT = 26;

inline int get_bit_index(const QModelIndex &index, bool little_endian) {
  return little_endian ? index.row() * 8 + 7 - index.column()
                       : index.row() * 8 + index.column();
}

BinaryView::BinaryView(QWidget *parent) : QTableView(parent) {
  model = new BinaryViewModel(this);
  setModel(model);
  delegate = new BinaryItemDelegate(this);
  setItemDelegate(delegate);
  horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  horizontalHeader()->hide();
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
  setFrameShape(QFrame::NoFrame);
  setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
  setMouseTracking(true);
}

void BinaryView::highlight(const Signal *sig) {
  if (sig != hovered_sig) {
    hovered_sig = sig;
    model->dataChanged(model->index(0, 0), model->index(model->rowCount() - 1, model->columnCount() - 1));
    emit signalHovered(hovered_sig);
  }
}

void BinaryView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags flags) {
  QModelIndex tl = indexAt({qMin(rect.left(), rect.right()), qMin(rect.top(), rect.bottom())});
  QModelIndex br = indexAt({qMax(rect.left(), rect.right()), qMax(rect.top(), rect.bottom())});
  if (!tl.isValid() || !br.isValid())
    return;

  if (tl < anchor_index) {
    br = anchor_index;
  } else if (anchor_index < br) {
    tl = anchor_index;
  }
  QItemSelection selection;
  for (int row = tl.row(); row <= br.row(); ++row) {
    int left_col = (row == tl.row()) ? tl.column() : 0;
    int right_col = (row == br.row()) ? br.column() : 7;
    selection.merge({model->index(row, left_col), model->index(row, right_col)}, flags);
  }
  selectionModel()->select(selection, flags);
}

void BinaryView::mousePressEvent(QMouseEvent *event) {
  delegate->setSelectionColor(style()->standardPalette().color(QPalette::Active, QPalette::Highlight));
  anchor_index = indexAt(event->pos());
  if (getResizingSignal() != nullptr) {
    auto item = (const BinaryViewModel::Item *)anchor_index.internalPointer();
    delegate->setSelectionColor(item->bg_color);
  }
  QTableView::mousePressEvent(event);
}

void BinaryView::mouseMoveEvent(QMouseEvent *event) {
  if (auto index = indexAt(event->pos()); index.isValid()) {
    auto item = (BinaryViewModel::Item *)index.internalPointer();
    const Signal *sig = item->sigs.isEmpty() ? nullptr : item->sigs.back();
    highlight(sig);
    sig ? QToolTip::showText(event->globalPos(), sig->name.c_str(), this, rect())
        : QToolTip::hideText();
  }
  QTableView::mouseMoveEvent(event);
}

void BinaryView::mouseReleaseEvent(QMouseEvent *event) {
  QTableView::mouseReleaseEvent(event);

  if (auto indexes = selectedIndexes(); !indexes.isEmpty()) {
    if (auto sig = getResizingSignal()) {
      auto release_index = indexAt(event->pos());
      auto [sig_from, sig_to] = getSignalRange(sig);
      if (release_index.column() == 8) {
        release_index = model->index(release_index.row(), 7);
      }
      int release_pos = get_bit_index(release_index, sig->is_little_endian);
      int archor_pos = get_bit_index(anchor_index, sig->is_little_endian);
      int start_bit, size;
      if (archor_pos == sig_from) {
        start_bit = release_pos;
        size = std::max(1, sig_to - start_bit + 1);
      } else {
        start_bit = sig_from;
        size = std::max(1, release_pos - start_bit + 1);
      }
      emit resizeSignal(sig, start_bit, size);
    } else {
      int from = indexes.first().row() * 8 + indexes.first().column();
      int to = indexes.back().row() * 8 + indexes.back().column();
      emit addSignal(from, to - from + 1);
    }
    clearSelection();
  }
  anchor_index = QModelIndex();
}

void BinaryView::leaveEvent(QEvent *event) {
  highlight(nullptr);
  QTableView::leaveEvent(event);
}

void BinaryView::setMessage(const QString &message_id) {
  model->setMessage(message_id);
  clearSelection();
  updateState();
}

const Signal *BinaryView::getResizingSignal() const {
  if (anchor_index.isValid()) {
    auto item = (const BinaryViewModel::Item *)anchor_index.internalPointer();
    if (item && item->sigs.size() > 0) {
      int archor_pos = anchor_index.row() * 8 + anchor_index.column();
      for (auto s : item->sigs) {
        auto [sig_from, sig_to] = getSignalRange(s);
        if (s->is_little_endian) {
          sig_from = bigEndianBitIndex(sig_from);
          sig_to = bigEndianBitIndex(sig_to);
        }
        if (archor_pos == sig_from || archor_pos == sig_to)
          return s;
      }
    }
  }
  return nullptr;
}

QSet<const Signal *> BinaryView::getOverlappingSignals() const {
  QSet<const Signal *> overlapping;
  for (int i = 0; i < model->rowCount(); ++i) {
    for (int j = 0; j < model->columnCount() - 1; ++j) {
      auto item = (const BinaryViewModel::Item *)model->index(i, j).internalPointer();
      if (item && item->sigs.size() > 1) {
        for (auto s : item->sigs)
          overlapping.insert(s);
      }
    }
  }
  return overlapping;
}

// BinaryViewModel

void BinaryViewModel::setMessage(const QString &message_id) {
  msg_id = message_id;

  beginResetModel();
  items.clear();
  row_count = 0;

  dbc_msg = dbc()->msg(msg_id);
  if (dbc_msg) {
    row_count = dbc_msg->size;
    items.resize(row_count * column_count);
    for (int i = 0; i < dbc_msg->sigs.size(); ++i) {
      const auto &sig = dbc_msg->sigs[i];
      auto [start, end] = getSignalRange(&sig);
      for (int j = start; j <= end; ++j) {
        int bit_index = sig.is_little_endian ? bigEndianBitIndex(j) : j;
        int idx = column_count * (bit_index / 8) + bit_index % 8;
        if (idx >= items.size()) {
          qWarning() << "signal " << sig.name.c_str() << "out of bounds.start_bit:" << sig.start_bit << "size:" << sig.size;
          break;
        }
        if (j == start) {
          sig.is_little_endian ? items[idx].is_lsb = true : items[idx].is_msb = true;
        }
        if (j == end) {
          sig.is_little_endian ? items[idx].is_msb = true : items[idx].is_lsb = true;
        }
        items[idx].bg_color = getColor(i);
        items[idx].sigs.push_back(&dbc_msg->sigs[i]);
      }
    }
  } else {
    row_count = can->lastMessage(msg_id).dat.size();
    items.resize(row_count * column_count);
  }

  endResetModel();
}

QModelIndex BinaryViewModel::index(int row, int column, const QModelIndex &parent) const {
  return createIndex(row, column, (void *)&items[row * column_count + column]);
}

Qt::ItemFlags BinaryViewModel::flags(const QModelIndex &index) const {
  return (index.column() == column_count - 1) ? Qt::ItemIsEnabled : Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

void BinaryViewModel::updateState() {
  auto prev_items = items;

  const auto &binary = can->lastMessage(msg_id).dat;
  // data size may changed.
  if (!dbc_msg && binary.size() != row_count) {
    beginResetModel();
    row_count = binary.size();
    items.clear();
    items.resize(row_count * column_count);
    endResetModel();
  }

  char hex[3] = {'\0'};
  for (int i = 0; i < std::min(binary.size(), row_count); ++i) {
    for (int j = 0; j < column_count - 1; ++j) {
      items[i * column_count + j].val = QChar((binary[i] >> (7 - j)) & 1 ? '1' : '0');
    }
    hex[0] = toHex(binary[i] >> 4);
    hex[1] = toHex(binary[i] & 0xf);
    items[i * column_count + 8].val = hex;
  }

  for (int i = 0; i < items.size(); ++i) {
    if (i >= prev_items.size() || prev_items[i].val != items[i].val) {
      auto idx = index(i / column_count, i % column_count);
      emit dataChanged(idx, idx);
    }
  }
}

QVariant BinaryViewModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (orientation == Qt::Vertical) {
    switch (role) {
      case Qt::DisplayRole: return section;
      case Qt::SizeHintRole: return QSize(30, CELL_HEIGHT);
      case Qt::TextAlignmentRole: return Qt::AlignCenter;
    }
  }
  return {};
}

// BinaryItemDelegate

BinaryItemDelegate::BinaryItemDelegate(QObject *parent) : QStyledItemDelegate(parent) {
  // cache fonts and color
  small_font.setPointSize(6);
  hex_font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
  hex_font.setBold(true);
  selection_color = QApplication::style()->standardPalette().color(QPalette::Active, QPalette::Highlight);
}

QSize BinaryItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
  QSize sz = QStyledItemDelegate::sizeHint(option, index);
  return {sz.width(), CELL_HEIGHT};
}

void BinaryItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
  auto item = (const BinaryViewModel::Item *)index.internalPointer();
  BinaryView *bin_view = (BinaryView *)parent();
  painter->save();

  bool hover = std::find_if(item->sigs.begin(), item->sigs.end(), [=](auto s) { return s == bin_view->hoveredSignal(); }) !=
               item->sigs.end();
  // background
  QColor bg_color = hover ? hoverColor(item->bg_color) : item->bg_color;
  if (option.state & QStyle::State_Selected) {
    bg_color = selection_color;
  }
  painter->fillRect(option.rect, bg_color);

  // text
  if (index.column() == 8) { // hex column
    painter->setFont(hex_font);
  } else if (hover) {
    painter->setPen(Qt::white);
  }
  painter->drawText(option.rect, Qt::AlignCenter, item->val);
  if (item->is_msb || item->is_lsb) {
    painter->setFont(small_font);
    painter->drawText(option.rect, Qt::AlignHCenter | Qt::AlignBottom, item->is_msb ? "MSB" : "LSB");
  }

  painter->restore();
}
