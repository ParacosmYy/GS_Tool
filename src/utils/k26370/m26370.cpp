#include "k26370/m26370.h"
QVector<double> m26370::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
