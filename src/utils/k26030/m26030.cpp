#include "k26030/m26030.h"
QVector<double> m26030::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
