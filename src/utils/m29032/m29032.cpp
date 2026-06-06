#include "m29032/m29032.h"
QVector<double> m29032::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
