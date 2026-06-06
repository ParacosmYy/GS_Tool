#include "g21506/m21506.h"
QVector<double> m21506::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
