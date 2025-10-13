import grpc
import asyncio
import argparse
import random
from pb import sample_pb2, sample_pb2_grpc

class GreeterServicer(sample_pb2_grpc.GreeterServicer):
    async def Greet(self, request, context):
        return sample_pb2.Response(result=f"Hello {request.data}")

    async def GreetClientStreaming(self, request_iterator, context):
        count = 0
        async for req in request_iterator:
            print(f"Received: {req.data}")
            count += 1
        return sample_pb2.Response(result=f"Received {count} messages")

    async def GreetServerStreaming(self, request, context):
        num = random.randint(1, 10)
        for i in range(num):
            yield sample_pb2.Response(result=f"Hello {request.data}")

    async def GreetBidi(self, request_iterator, context):
        async for req in request_iterator:
            yield sample_pb2.Response(result=f"Hello: {req.data}")

async def serve(port):
    server = grpc.aio.server()
    sample_pb2_grpc.add_GreeterServicer_to_server(GreeterServicer(), server)
    server.add_insecure_port(f'[::]:{port}')
    await server.start()
    print(f"Server started on port {port}")
    await server.wait_for_termination()

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Run gRPC server')
    parser.add_argument('--port', type=int, default=12345, help='Port to listen on')
    args = parser.parse_args()
    asyncio.run(serve(args.port))
