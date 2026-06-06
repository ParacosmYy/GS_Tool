#include "h35207/m35207.h"
QVector<double> m35207::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
