#include "k26070/m26070.h"
QVector<double> m26070::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
