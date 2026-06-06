#include "b15801/m15801.h"
QVector<double> m15801::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
