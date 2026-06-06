#include "m24272/m24272.h"
QVector<double> m24272::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
