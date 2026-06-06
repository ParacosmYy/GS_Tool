#include "b8501/m8501.h"
QVector<double> m8501::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
