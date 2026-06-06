#include "m30912/m30912.h"
QVector<double> m30912::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
