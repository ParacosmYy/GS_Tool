#include "m14032/m14032.h"
QVector<double> m14032::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
