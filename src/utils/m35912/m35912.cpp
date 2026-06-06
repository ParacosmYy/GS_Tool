#include "m35912/m35912.h"
QVector<double> m35912::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
