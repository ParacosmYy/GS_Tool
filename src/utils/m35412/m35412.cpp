#include "m35412/m35412.h"
QVector<double> m35412::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
