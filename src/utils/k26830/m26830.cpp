#include "k26830/m26830.h"
QVector<double> m26830::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
