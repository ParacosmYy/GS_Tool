#include "k24310/m24310.h"
QVector<double> m24310::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
