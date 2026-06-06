#include "i29708/m29708.h"
QVector<double> m29708::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
