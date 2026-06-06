#include "k26810/m26810.h"
QVector<double> m26810::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
