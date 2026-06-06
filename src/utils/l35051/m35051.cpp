#include "l35051/m35051.h"
QVector<double> m35051::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
