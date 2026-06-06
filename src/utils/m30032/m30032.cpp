#include "m30032/m30032.h"
QVector<double> m30032::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
