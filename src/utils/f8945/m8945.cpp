#include "f8945/m8945.h"
QVector<double> m8945::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
