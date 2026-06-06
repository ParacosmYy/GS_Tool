#include "a25140/m25140.h"
QVector<double> m25140::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
