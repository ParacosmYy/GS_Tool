#include "b9801/m9801.h"
QVector<double> m9801::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
