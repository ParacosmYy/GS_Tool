#include "o15254/m15254.h"
QVector<double> m15254::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
