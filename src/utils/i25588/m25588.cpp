#include "i25588/m25588.h"
QVector<double> m25588::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
