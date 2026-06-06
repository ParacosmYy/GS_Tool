#include "m19912/m19912.h"
QVector<double> m19912::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
