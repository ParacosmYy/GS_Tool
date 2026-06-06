#include "k24270/m24270.h"
QVector<double> m24270::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
