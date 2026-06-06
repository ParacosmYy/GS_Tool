#include "b26021/m26021.h"
QVector<double> m26021::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
