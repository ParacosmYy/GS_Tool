#include "k26510/m26510.h"
QVector<double> m26510::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
