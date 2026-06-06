#include "i28188/m28188.h"
QVector<double> m28188::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
