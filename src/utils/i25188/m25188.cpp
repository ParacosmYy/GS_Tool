#include "i25188/m25188.h"
QVector<double> m25188::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
