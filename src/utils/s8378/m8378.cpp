#include "s8378/m8378.h"
QVector<double> m8378::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
