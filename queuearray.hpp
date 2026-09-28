/*
  Queue implemented in an array
  template <typename D, typename I>
  D = data type
  I = index type (uint8_t for max queue size of 254 items, uint16_t for max 65535, etc...)

functions:
  constructor([len]): len = size of queue.
  begin([len]): len = size of queue.  clear queue
  clear: empty the queue
  full: return true if queue is full
  empty: return true if queue is empty
  push: add one item to end of queue
  unpush: pull item from tail of queue
  peektail: peek at tail item
  peek: return the item that is at the begining of queue without removing it
  pull: pull one item from begining of queue
  unpull: put back one item at begining of queue
    if item is not given, then restore previous item that was there
  available: return the number of items currently stored in the queue
  space: return the number of items that can be added to make the queue full
  
*/


#include "QueueArray.h"

template<typename D, typename I> QueueArray<D, I>::QueueArray(I len) {
  buf = 0;
  bufn = 0;
  begin(len);
}
template<typename D, typename I> QueueArray<D, I>::~QueueArray() {
  if (buf) delete buf;
}
template<typename D, typename I> void QueueArray<D, I>::begin(I len) {
  if (len > 0) {
    if (buf) delete buf;
    buf = new (D[len + 1]);
    bufn = (buf != 0) ? len + 1 : 0;
  }
  clear();
}
template<typename D, typename I> void QueueArray<D, I>::clear() {
  head = 0;
  tail = 0;
}
template<typename D, typename I> bool QueueArray<D, I>::full() {
  return head == inc(tail);
}
template<typename D, typename I> bool QueueArray<D, I>::empty() {
  return head == tail;
}
template<typename D, typename I> void QueueArray<D, I>::push(D d) {
  if (!full()) {
    buf[tail] = d;
    tail = inc(tail);
  }
}
template<typename D, typename I> D QueueArray<D, I>::unpush() {
  if (empty()) return 0xFF;
  tail = dec(tail);
  return buf[tail];
}
template<typename D, typename I> D QueueArray<D, I>::peektail() {
  return buf[dec(tail)];
}
template<typename D, typename I> D QueueArray<D, I>::pull() {
  if (empty()) return 0xFF;
  D d = buf[head];
  head = inc(head);
  return d;
}
template<typename D, typename I> D QueueArray<D, I>::peek() {
  return buf[head];
}
template<typename D, typename I> void QueueArray<D, I>::unpull() {
  if (!full()) head = dec(head);
}
template<typename D, typename I> void QueueArray<D, I>::unpull(D d) {
  if (!full()) {
    head = dec(head);
    buf[head] = d;
  }
}
template<typename D, typename I> I QueueArray<D, I>::available() {
  return ((tail >= head) ? 0 : bufn) + tail - head;
}
template<typename D, typename I> I QueueArray<D, I>::space() {
  return bufn - 1 - available();
}
template<typename D, typename I> I QueueArray<D, I>::inc(I i) {
  ++i;
  if (i >= bufn) i = 0;
  return i;
}
template<typename D, typename I> I QueueArray<D, I>::dec(I i) {
  if (i == 0) i = bufn;
  return --i;
}

//
