#include "k26910/m26910.h"
QVector<double> m26910::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
