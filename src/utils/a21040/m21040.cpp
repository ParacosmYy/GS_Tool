#include "a21040/m21040.h"
QVector<double> m21040::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
