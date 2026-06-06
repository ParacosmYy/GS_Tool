#include "a35880/m35880.h"
QVector<double> m35880::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
