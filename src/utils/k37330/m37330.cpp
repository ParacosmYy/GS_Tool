#include "k37330/m37330.h"
QVector<double> m37330::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
