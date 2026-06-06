#include "a24620/m24620.h"
QVector<double> m24620::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
