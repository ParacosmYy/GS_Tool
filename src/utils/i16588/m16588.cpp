#include "i16588/m16588.h"
QVector<double> m16588::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
