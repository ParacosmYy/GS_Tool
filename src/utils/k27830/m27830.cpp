#include "k27830/m27830.h"
QVector<double> m27830::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
