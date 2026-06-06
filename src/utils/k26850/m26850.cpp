#include "k26850/m26850.h"
QVector<double> m26850::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
