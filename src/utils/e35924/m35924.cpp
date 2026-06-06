#include "e35924/m35924.h"
QVector<double> m35924::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
