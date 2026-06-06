#include "i27608/m27608.h"
QVector<double> m27608::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
