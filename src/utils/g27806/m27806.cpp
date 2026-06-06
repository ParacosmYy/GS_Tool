#include "g27806/m27806.h"
QVector<double> m27806::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
