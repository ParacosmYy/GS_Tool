#include "b24801/m24801.h"
QVector<double> m24801::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
