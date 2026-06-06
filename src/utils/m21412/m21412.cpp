#include "m21412/m21412.h"
QVector<double> m21412::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
