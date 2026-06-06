#include "l35651/m35651.h"
QVector<double> m35651::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
