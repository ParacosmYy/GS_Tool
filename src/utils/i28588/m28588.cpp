#include "i28588/m28588.h"
QVector<double> m28588::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
