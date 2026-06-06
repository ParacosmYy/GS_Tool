#include "i28828/m28828.h"
QVector<double> m28828::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
