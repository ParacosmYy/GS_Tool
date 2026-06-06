#include "k24830/m24830.h"
QVector<double> m24830::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
