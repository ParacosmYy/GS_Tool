#include "m9412/m9412.h"
QVector<double> m9412::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
