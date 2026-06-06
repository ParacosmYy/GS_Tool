#include "k26410/m26410.h"
QVector<double> m26410::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
