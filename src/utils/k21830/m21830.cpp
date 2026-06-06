#include "k21830/m21830.h"
QVector<double> m21830::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
