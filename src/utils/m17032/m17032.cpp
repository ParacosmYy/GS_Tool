#include "m17032/m17032.h"
QVector<double> m17032::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
