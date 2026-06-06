#include "o8354/m8354.h"
QVector<double> m8354::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
