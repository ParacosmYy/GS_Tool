#include "b7801/m7801.h"
QVector<double> m7801::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
