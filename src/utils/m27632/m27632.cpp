#include "m27632/m27632.h"
QVector<double> m27632::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
