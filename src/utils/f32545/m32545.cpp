#include "f32545/m32545.h"
QVector<double> m32545::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
