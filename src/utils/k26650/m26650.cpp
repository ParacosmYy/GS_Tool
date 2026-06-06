#include "k26650/m26650.h"
QVector<double> m26650::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
