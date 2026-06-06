#include "k26210/m26210.h"
QVector<double> m26210::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
