#include "c28022/m28022.h"
QVector<double> m28022::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
