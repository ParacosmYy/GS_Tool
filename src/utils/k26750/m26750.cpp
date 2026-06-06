#include "k26750/m26750.h"
QVector<double> m26750::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
