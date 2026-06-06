#include "l18231/m18231.h"
QVector<double> m18231::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
